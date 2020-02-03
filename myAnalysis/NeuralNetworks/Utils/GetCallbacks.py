#xxx

# //--------------------------------------------
import keras
from tensorflow.keras.callbacks import TensorBoard, EarlyStopping, LambdaCallback, LearningRateScheduler, ReduceLROnPlateau
from Utils.Helper import TimeHistory
# //--------------------------------------------
# //--------------------------------------------


# Define learning rate schedule
def step_decay(epoch):
    initial_lrate = 0.05
    drop = 0.5
    epochs_drop = 20
    lrate = initial_lrate * math.pow(drop, math.floor((1+epoch)/epochs_drop))
    print("== Epoch "+str(epoch+1)+" : learning rate = "+str(lrate) )
    return lrate





# //--------------------------------------------
# //--------------------------------------------
# //--------------------------------------------
 ######     ###    ##       ##       ########     ###     ######  ##    ##  ######
##    ##   ## ##   ##       ##       ##     ##   ## ##   ##    ## ##   ##  ##    ##
##        ##   ##  ##       ##       ##     ##  ##   ##  ##       ##  ##   ##
##       ##     ## ##       ##       ########  ##     ## ##       #####     ######
##       ######### ##       ##       ##     ## ######### ##       ##  ##         ##
##    ## ##     ## ##       ##       ##     ## ##     ## ##    ## ##   ##  ##    ##
 ######  ##     ## ######## ######## ########  ##     ##  ######  ##    ##  ######
# //--------------------------------------------
# //--------------------------------------------
# //--------------------------------------------


def Get_Callbacks(weight_dir):

    # batchLogCallback = LambdaCallback(on_batch_end=batchOutput) #Could be used to perform action at end of each batch

    #Create logfile for Tensorboard, allowing to get visualization of training/test metrics
    #Usage : tensorboard --logdir=/full_path_to_your_logs --port 0
    dirlog = weight_dir + 'logs'
    tensorboard = TensorBoard(log_dir=dirlog, histogram_freq=0, write_graph=True, write_images=True)

    #Reduce learning rate when reach metrics plateau
    lrate_plateau = ReduceLROnPlateau(monitor='val_loss', factor=0.5, patience=10, verbose=1, mode='auto', min_delta=1e-4, cooldown=0, min_lr=1e-6)
    # lrate_plateau = keras.callbacks.ReduceLROnPlateau(monitor='val_loss', factor=0.5, patience=10, verbose=1, mode='auto', min_delta=1e-4, cooldown=0, min_lr=1e-6)

    # keras.callbacks.callbacks.ReduceLROnPlateau(monitor='val_loss', factor=0.1, patience=10, verbose=0, mode='auto', min_delta=0.0001, cooldown=0, min_lr=0)

    #NB : ES takes place when monitored quantity has not improved **WRT BEST VALUE YET** for a number 'patience' of epochs
    # ES = keras.callbacks.EarlyStopping(monitor='val_loss', min_delta=1e-4, patience=100, verbose=1, restore_best_weights=True, mode='auto') #Try early stopping after N epochs without metrics update # monitor='val_loss'
    ES = EarlyStopping(monitor='val_loss', min_delta=1e-4, patience=100, verbose=1, restore_best_weights=True, mode='auto') #Try early stopping after N epochs without metrics update # monitor='val_loss'

    #Reduce learning rate when a metric has stopped improving
    #NB : Do not manually set learning rate (ex: model.optimizer.lr = 3e-4) when using ReduceLROnPlateau().
    lrate_sched = LearningRateScheduler(step_decay)

    #Get training time at each epoch
    time_callback = TimeHistory()

    # callbacks_list = [tensorboard, ES]
    list = [tensorboard, lrate_plateau, time_callback]

    return list
